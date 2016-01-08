import java.io.IOException;
import java.util.*;

import org.apache.hadoop.conf.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapreduce.*;
import org.apache.hadoop.mapreduce.lib.input.*;
import org.apache.hadoop.mapreduce.lib.output.*;

public class Run {

    public static class MMapper
        extends Mapper<LongWritable, Text, Text, Text>{

        public void map(LongWritable key, Text value, Context context
                    ) throws IOException, InterruptedException {
            Configuration conf = context.getConfiguration();
            int ni = Integer.parseInt(conf.get("ni"));
            int nj = Integer.parseInt(conf.get("nj"));
            int nk = Integer.parseInt(conf.get("nk"));
            String[] ins = value.toString().split(" ");
            Text okey = new Text();
            Text oval = new Text();
            if (ins[0].equals("0")) {
                for (int j = 0; j < nj; j++) {
                    okey.set(ins[1] + " " + j);
                    oval.set(ins[2] + " " + ins[3]);
                    context.write(okey, oval);
                }
            } else {
                for (int i = 0; i < ni; i++) {
                    okey.set(i + " " + ins[2]);
                    oval.set(ins[1] + " " + ins[3]);
                    context.write(okey, oval);
                }
            }
        }
    }

    public static class MReducer
        extends Reducer<Text, Text, Text, Text> {

        public void reduce(Text key, Iterable<Text> values, Context context)
                throws IOException, InterruptedException {
            int sum = 0;
            Configuration conf = context.getConfiguration();
            int nk = Integer.parseInt(conf.get("nk"));
            HashMap<Integer, Integer> hm = new HashMap<Integer, Integer>();
            for (Text val : values) {
                String[] vals = val.toString().split(" ");
                int k = Integer.parseInt(vals[0]);
                int v = Integer.parseInt(vals[1]);
                if (hm.get(k) != null) {
                    sum += v * hm.get(k);
                } else  {
                    hm.put(k, v);
                }
            }
            context.write(key, new Text(((Integer)sum).toString()));
        }
    }

    public static void main(String[] args) throws Exception {
        Configuration conf = new Configuration();
        conf.set("ni", "100");
        conf.set("nk", "50");
        conf.set("nj", "100");
        Job job = Job.getInstance(conf, "word count");
        job.setJarByClass(Run.class);
        job.setMapperClass(MMapper.class);
        //job.setCombinerClass(MReducer.class);
        job.setReducerClass(MReducer.class);
        
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(Text.class);
        
        job.setInputFormatClass(TextInputFormat.class);
        job.setOutputFormatClass(TextOutputFormat.class);
        
        FileInputFormat.addInputPath(job, new Path(args[0]));
        FileOutputFormat.setOutputPath(job, new Path(args[1]));

        System.exit(job.waitForCompletion(true) ? 0 : 1);
    }
}
